#include <gtest/gtest.h>
#include <stdbool.h>

extern "C"{
#include <japi.h>
#include <japi_intern.h>
#include <japi_pushsrv.h>
#include <japi_utils.h>
#include <rw_n.h>
}

/* The handler for japi_register_request test */
static void dummy_request_handler(japi_context *ctx, json_object *request, json_object *response)
{
	/* Not existent dummy request */
	json_object_object_add(response,"value",json_object_new_string("hello world"));
}

TEST(JAPI,Init)
{
	/* On success, a japi_context object is returned. On error, NULL is returned */
	EXPECT_TRUE(japi_init(NULL) != NULL);
}

TEST(JAPI,GetValueAsStringAndBool)
{
	json_object *jresp;

	jresp = json_object_new_object();
	json_object_object_add(jresp,"key",json_object_new_string("value"));
	json_object_object_add(jresp,"bool",json_object_new_boolean(TRUE));

	/* On success, string is returned. On error, NULL is returned */
	EXPECT_STREQ(japi_get_value_as_str(jresp,"key"),"value");
	/* On success, bool is returned. On error, NULL is returned */
	EXPECT_TRUE(japi_get_value_as_bool(jresp,"bool"));

	/* Clean up */
	json_object_put(jresp);
}

TEST(JAPI,ProcessMessage)
{
	japi_context *ctx;
	char* response;
	const char* request;
	json_object *jobj;
	int socket;

	jobj = json_object_new_object();
	request = "{'japi_request':'dummy_request_handler'}";
	response = NULL;
	ctx = japi_init(NULL);
	socket = 4;

	japi_register_request(ctx,"dummy_request_handler",&dummy_request_handler);
	/* On success, 0 returned. On error, -1 is returned */
	EXPECT_EQ(japi_process_message(ctx, request, &response, socket),0);
	jobj = json_tokener_parse(response);
	EXPECT_STREQ("hello world",japi_get_value_as_str(jobj,"value"));

	/* Clean up */
	japi_destroy(ctx);
}

TEST(JAPI,Register)
{
	japi_context *ctx;
	ctx = japi_init(NULL);

	/* On success, zero is returned. On error, -1..-4 is returned */
	EXPECT_EQ(japi_register_request(ctx,"req_name",&dummy_request_handler),0);
	EXPECT_EQ(japi_register_request(NULL,"req_name",&dummy_request_handler),-1);
	EXPECT_EQ(japi_register_request(ctx,NULL,&dummy_request_handler),-2);
	EXPECT_EQ(japi_register_request(ctx,"req_name",NULL),-3);

	/* Registering the same request name again or an empty request name, should not be possible */
	EXPECT_EQ(japi_register_request(ctx,"req_name",&dummy_request_handler),-5);
	EXPECT_EQ(japi_register_request(ctx,"dummy_request_02",&dummy_request_handler),0); // same handler for another name
	EXPECT_EQ(japi_register_request(ctx,"",&dummy_request_handler),-2);

	japi_destroy(ctx);
}

TEST(JAPI_Push_Service,Register)
{
	japi_context *ctx;
	ctx = japi_init(NULL);

	/* On success, a pointer to the japi_push_service context is returned. On error, NULL is returned */
	EXPECT_TRUE(japi_pushsrv_register(ctx,"test_pushsrv") != NULL);
	EXPECT_TRUE(japi_pushsrv_register(NULL,"test_pushsrv") == NULL);
	EXPECT_TRUE(japi_pushsrv_register(ctx,NULL) == NULL);

	/* Registering the same push service name again or an empty push service name, should not be possible */
	EXPECT_TRUE(japi_pushsrv_register(ctx,"test_pushsrv") == NULL);
	EXPECT_TRUE(japi_pushsrv_register(ctx,"") == NULL);

	/* Clean up */
	japi_destroy(ctx);
}

TEST(JAPI_Push_Service,SubscribeAndUnsubscribe)
{
	int socket;
	char* pushsrv_name;
	japi_context *ctx;
	json_object *jresp;

	socket = 4;
	pushsrv_name = (char*)"test_pushsrv";
	jresp = json_object_new_object();
	ctx = japi_init(NULL);

	/* Sub-/unsubscribe before registering, expecting false */
	japi_pushsrv_subscribe(ctx,socket,pushsrv_name,jresp);
	EXPECT_FALSE(japi_get_value_as_bool(jresp, "success"));

	japi_pushsrv_unsubscribe(ctx,socket,pushsrv_name,jresp);
	EXPECT_FALSE(japi_get_value_as_bool(jresp, "success"));

	/* Try to unsubscribe without subcribed before, should fail */
	japi_pushsrv_register(ctx,"test_pushsrv");
	japi_pushsrv_unsubscribe(ctx,socket,pushsrv_name,jresp);
	EXPECT_FALSE(japi_get_value_as_bool(jresp, "success"));

	/* Expect true */
	japi_pushsrv_subscribe(ctx,socket,pushsrv_name,jresp);
	EXPECT_TRUE(japi_get_value_as_bool(jresp, "success"));

	japi_pushsrv_unsubscribe(ctx,socket,pushsrv_name,jresp);
	EXPECT_TRUE(japi_get_value_as_bool(jresp, "success"));

	/* Clean up */
	json_object_put(jresp);

}

TEST(JAPI_Push_Service,List)
{
	japi_context *ctx;
	japi_pushsrv_context *psc;
	json_object *jobj;

	ctx = japi_init(NULL);
	jobj = json_object_new_object();

	/* Register some test services */
	japi_pushsrv_register(ctx,"test01");
	japi_pushsrv_register(ctx,"test02");
	japi_pushsrv_register(ctx,"test03");

	/* The function to be tested */
	japi_pushsrv_list(ctx, NULL, jobj);

	psc = ctx->push_services;

	/* Iterate push service array & context and compare strings */
	json_object_object_foreach(jobj, key, val) {
		json_object_object_get_ex(jobj, key, &val);
		int arraylen = json_object_array_length(val);
		int i;
		json_object * jvalue;
		while (psc != NULL) {
			for (i=0; i< arraylen; i++) {
				jvalue = json_object_array_get_idx(val, i);
				EXPECT_STREQ(json_object_get_string(jvalue),psc->pushsrv_name);
				psc = psc->next;
			}
		}
	}

	/* Clean up */
	japi_destroy(ctx);
	json_object_put(jobj);
}
